#ifndef INPUTS_HDR
#define INPUTS_HDR
class Inputs
{
    public:
        enum Key : long
        {	ESC = 1,
            LMB = 2, RMB = 4, MMB = 8,
            W = 16, A = 32, S = 64, D = 128,
            ALT = 256, ENTER = 512
        };

        struct mouseStatus
        {
            double x, y, scroll; 
        };

        double mouseSensitivity{0.1};      
 
        //signal to close app eg. clicking the cross icon
        bool close {false};

        mouseStatus getMouseStatus()
        { return {mouseX, mouseY, mouseScroll};  }

        void setMousePosition(double x, double y)
        {
            mouseX = mouseSensitivity * x; 
            mouseY = mouseSensitivity * y; 
        }

        void setMouseScroll(double s)
        {
            mouseScroll = s;
        }

        void press(Key key)
        {
            keys |= key;
        }

        void release(Key key)
        {
            keys &= ~key;
        }

        template <typename... Args>
        bool pressed(Args... args) const
        {
            long mask = composeKeys(args...);
            return (keys & mask) == mask;
        }
        template <typename T, typename... Args>
        long composeKeys(T keyFlag, Args... args) const
        {
            return keyFlag | composeKeys(args...);
        }
        template <typename T>
        long composeKeys(T t) const
        {
            return t;
        }
    private:
        //expecting max 32 keys, plus possible modifiers
        long keys {0};
        double mouseX {0}, mouseY {0}, mouseScroll {0};
};

#endif
