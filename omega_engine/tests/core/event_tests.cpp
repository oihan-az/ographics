#include <omega/core/event.hpp>

#include <gtest/gtest.h>

namespace
{

struct TestEvent
{
    int value;
};

struct AnotherEvent
{
    int value;
};

struct NewEvent
{
    int value;
};

} // namespace

TEST(EventSystemTests, PublishAndDispatch)
{
    omega::EventSystem events;

    int received_value = 0;

    events.subscribe<TestEvent>([&](const TestEvent& event) { received_value = event.value; });

    events.publish(TestEvent{42});

    EXPECT_EQ(received_value, 0);

    events.dispatch();

    EXPECT_EQ(received_value, 42);
}

TEST(EventSystemTests, MultipleSubscribers)
{
    omega::EventSystem events;

    int first_value = 0;
    int second_value = 0;

    events.subscribe<TestEvent>([&](const TestEvent& event) { first_value = event.value; });
    events.subscribe<TestEvent>([&](const TestEvent& event) { second_value = event.value; });

    events.publish(TestEvent{42});
    events.dispatch();

    EXPECT_EQ(first_value, 42);
    EXPECT_EQ(second_value, 42);

    // Subscriptions persist across dispatch cycles
    events.publish(TestEvent{100});
    events.dispatch();

    EXPECT_EQ(first_value, 100);
    EXPECT_EQ(second_value, 100);
}

TEST(EventSystemTests, Unsubscribe)
{
    omega::EventSystem events;

    int received_value = 0;

    auto subscription = events.subscribe<TestEvent>([&](const TestEvent& event) { received_value = event.value; });

    events.publish(TestEvent{42});
    events.dispatch();

    EXPECT_EQ(received_value, 42);

    events.unsubscribe(subscription);

    events.publish(TestEvent{100});
    events.dispatch();

    // Unsubscribed listeners must no longer receive events
    EXPECT_EQ(received_value, 42);
}

TEST(EventSystemTests, EventsPublishedDuringDispatchAreDeferred)
{
    omega::EventSystem events;

    int received_count = 0;

    events.subscribe<TestEvent>(
        [&](const TestEvent&)
        {
            ++received_count;

            if (received_count == 1)
                events.publish(TestEvent{2});
        });

    events.publish(TestEvent{1});

    events.dispatch();

    // The newly published event must wait for the next cycle
    EXPECT_EQ(received_count, 1);

    events.dispatch();

    EXPECT_EQ(received_count, 2);
}

TEST(EventSystemTests, DifferentEventTypesAreDeferred)
{
    omega::EventSystem events;

    int received_count = 0;

    events.subscribe<TestEvent>([&](const TestEvent&) { events.publish(AnotherEvent{42}); });
    events.subscribe<AnotherEvent>([&](const AnotherEvent&) { ++received_count; });

    events.publish(TestEvent{1});

    events.dispatch();

    // Events of other types published during dispatch are deferred too
    EXPECT_EQ(received_count, 0);

    events.dispatch();

    EXPECT_EQ(received_count, 1);
}

TEST(EventSystemTests, UnsubscribeDuringDispatch)
{
    omega::EventSystem events;

    int received_count = 0;

    uint64_t subscription = 0;

    subscription = events.subscribe<TestEvent>(
        [&](const TestEvent&)
        {
            ++received_count;

            // Unsubscribe after receiving the first event
            events.unsubscribe(subscription);
        });

    events.publish(TestEvent{1});
    events.publish(TestEvent{2});

    events.dispatch();

    // The second event must not reach the unsubscribed listener
    EXPECT_EQ(received_count, 1);
}

TEST(EventSystemTests, SubscriptionsDuringDispatchAreDeferred)
{
    omega::EventSystem events;

    int first_count = 0;
    int second_count = 0;

    events.subscribe<TestEvent>(
        [&](const TestEvent&)
        {
            ++first_count;

            if (first_count == 1)
            {
                events.subscribe<TestEvent>([&](const TestEvent&) { ++second_count; });

                events.publish(TestEvent{2});
            }
        });

    events.publish(TestEvent{1});
    events.dispatch();

    EXPECT_EQ(first_count, 1);
    EXPECT_EQ(second_count, 0);

    events.dispatch();

    EXPECT_EQ(first_count, 2);
    EXPECT_EQ(second_count, 1);
}

TEST(EventSystemTests, RecursiveDispatchIsIgnored)
{
    omega::EventSystem events;

    int received_count = 0;

    events.subscribe<TestEvent>(
        [&](const TestEvent& event)
        {
            ++received_count;

            if (event.value == 1)
            {
                events.publish(TestEvent{2});
                events.dispatch();
            }
        });

    events.publish(TestEvent{1});
    events.dispatch();

    // Recursive dispatch must not process the newly published event
    EXPECT_EQ(received_count, 1);

    events.dispatch();

    EXPECT_EQ(received_count, 2);
}

TEST(EventSystemTests, NewEventTypesAreDeferred)
{
    omega::EventSystem events;

    int received_value = 0;

    events.subscribe<TestEvent>(
        [&](const TestEvent&)
        {
            // Creates a new event queue during dispatch
            events.publish(NewEvent{42});
        });

    events.publish(TestEvent{1});
    events.dispatch();

    // Subscribe after the new event has been queued
    events.subscribe<NewEvent>([&](const NewEvent& event) { received_value = event.value; });

    EXPECT_EQ(received_value, 0);

    events.dispatch();

    EXPECT_EQ(received_value, 42);
}

TEST(EventSystemTests, EventsAreDispatchedInOrder)
{
    omega::EventSystem events;

    std::vector<int> received_values;

    events.subscribe<TestEvent>([&](const TestEvent& event) { received_values.push_back(event.value); });

    events.publish(TestEvent{1});
    events.publish(TestEvent{2});
    events.publish(TestEvent{3});

    events.dispatch();

    ASSERT_EQ(received_values.size(), 3);
    EXPECT_EQ(received_values[0], 1);
    EXPECT_EQ(received_values[1], 2);
    EXPECT_EQ(received_values[2], 3);
}

TEST(EventSystemTests, MultipleEventTypes)
{
    omega::EventSystem events;

    int test_value = 0;
    int another_value = 0;

    events.subscribe<TestEvent>([&](const TestEvent& event) { test_value = event.value; });

    events.subscribe<AnotherEvent>([&](const AnotherEvent& event) { another_value = event.value; });

    events.publish(TestEvent{10});
    events.publish(AnotherEvent{20});

    events.dispatch();

    EXPECT_EQ(test_value, 10);
    EXPECT_EQ(another_value, 20);
}

TEST(EventSystemTests, UnsubscribeAnotherListenerDuringDispatch)
{
    omega::EventSystem events;

    int first_count = 0;
    int second_count = 0;

    uint64_t second_subscription = 0;

    events.subscribe<TestEvent>(
        [&](const TestEvent&)
        {
            ++first_count;
            events.unsubscribe(second_subscription);
        });

    second_subscription = events.subscribe<TestEvent>([&](const TestEvent&) { ++second_count; });

    events.publish(TestEvent{1});
    events.dispatch();

    EXPECT_EQ(first_count, 1);
    EXPECT_EQ(second_count, 0);
}

TEST(EventSystemTests, EventsWithoutListenersAreDiscarded)
{
    omega::EventSystem events;

    events.publish(TestEvent{42});
    events.dispatch();

    int received_value = 0;

    events.subscribe<TestEvent>([&](const TestEvent& event) { received_value = event.value; });

    events.dispatch();

    // A listener registered later must not receive an already-dispatched event
    EXPECT_EQ(received_value, 0);
}