#include <gtest/gtest.h>

#include <omega/core/event/event_system.hpp>

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

TEST(EventSystemTests, PublishAndSwap)
{
    omega::EventSystem events;

    events.publish(TestEvent{42});

    // Before swapping, the processing queue must remain completely empty
    EXPECT_TRUE(events.get_queue<TestEvent>().get_events().empty());

    // Perform the frame buffer swap
    events.swap_all_buffers();

    // Now events must be safely available for linear data-oriented reading
    const auto& queue = events.get_queue<TestEvent>().get_events();
    ASSERT_EQ(queue.size(), 1);
    EXPECT_EQ(queue[0].value, 42);
}

TEST(EventSystemTests, EventsPersistForMultipleSystems)
{
    omega::EventSystem events;

    events.publish(TestEvent{42});
    events.swap_all_buffers();

    // Verify that multiple detached systems can read the same vector without destroying memory
    const auto& system_one_view = events.get_queue<TestEvent>().get_events();
    const auto& system_two_view = events.get_queue<TestEvent>().get_events();

    ASSERT_EQ(system_one_view.size(), 1);
    EXPECT_EQ(system_one_view[0].value, 42);

    ASSERT_EQ(system_two_view.size(), 1);
    EXPECT_EQ(system_two_view[0].value, 42);
}

TEST(EventSystemTests, BuffersAreClearedOnNextSwap)
{
    omega::EventSystem events;

    events.publish(TestEvent{42});
    events.swap_all_buffers();

    EXPECT_EQ(events.get_queue<TestEvent>().get_events().size(), 1);

    // A second swap sequence without any new publications must fully clear the active processing queue
    events.swap_all_buffers();
    EXPECT_TRUE(events.get_queue<TestEvent>().get_events().empty());
}

TEST(EventSystemTests, EventsPublishedDuringProcessingAreDeferred)
{
    omega::EventSystem events;

    events.publish(TestEvent{1});
    events.swap_all_buffers();

    // Simulate an isolated system processing the frozen queue for Frame 1
    const auto& processing_queue = events.get_queue<TestEvent>().get_events();
    ASSERT_EQ(processing_queue.size(), 1);

    // Reacting to Event 1 publishes Event 2 (this must go straight to the incoming buffer)
    if (processing_queue[0].value == 1)
    {
        events.publish(TestEvent{2});
    }

    // Event 2 must NOT bleed into the current processing queue of the active frame execution loop
    EXPECT_EQ(events.get_queue<TestEvent>().get_events().size(), 1);

    // Progress to the next simulation frame tick
    events.swap_all_buffers();

    // The deferred secondary event now securely shifts into the active processing queue
    const auto& next_processing_queue = events.get_queue<TestEvent>().get_events();
    ASSERT_EQ(next_processing_queue.size(), 1);
    EXPECT_EQ(next_processing_queue[0].value, 2);
}

TEST(EventSystemTests, DifferentEventTypesAreIndependent)
{
    omega::EventSystem events;

    events.publish(TestEvent{10});
    events.publish(AnotherEvent{20});

    events.swap_all_buffers();

    const auto& test_queue = events.get_queue<TestEvent>().get_events();
    const auto& another_queue = events.get_queue<AnotherEvent>().get_events();

    ASSERT_EQ(test_queue.size(), 1);
    EXPECT_EQ(test_queue[0].value, 10);

    ASSERT_EQ(another_queue.size(), 1);
    EXPECT_EQ(another_queue[0].value, 20);
}

TEST(EventSystemTests, NewEventTypesAreCreatedDynamically)
{
    omega::EventSystem events;

    // Force the dynamic compilation infrastructure to generate a completely new type pool at runtime
    events.publish(NewEvent{99});
    events.swap_all_buffers();

    const auto& new_queue = events.get_queue<NewEvent>().get_events();
    ASSERT_EQ(new_queue.size(), 1);
    EXPECT_EQ(new_queue[0].value, 99);
}

TEST(EventSystemTests, EventsArePreservedInOrder)
{
    omega::EventSystem events;

    events.publish(TestEvent{1});
    events.publish(TestEvent{2});
    events.publish(TestEvent{3});

    events.swap_all_buffers();

    const auto& queue = events.get_queue<TestEvent>().get_events();

    // Order must match ingestion exactly
    ASSERT_EQ(queue.size(), 3);
    EXPECT_EQ(queue[0].value, 1);
    EXPECT_EQ(queue[1].value, 2);
    EXPECT_EQ(queue[2].value, 3);
}