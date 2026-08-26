#ifndef OGFX_PRESENT_MODE_HPP
#define OGFX_PRESENT_MODE_HPP

namespace ogfx
{

enum class PresentMode
{
    Immediate,
    FIFO,
    Mailbox,
};

const char* to_string(PresentMode mode);

} // namespace ogfx

#endif // OGFX_PRESENT_MODE_HPP