
class Inputs
{
    public:
        enum Keys {	ESC = 1,
            LMB = 2, RMB = 4, MMB = 8,
            W = 16, A = 32, S = 64, D = 128,
            ALT = 256, ENTER = 512};

        void press(Keys key)
        {
            keys |= key;
        }

        void release(Keys key)
        {
            keys &= ~key;
        }

        template <typename... Args>
            bool pressed(Args... args) 
            {
                return keys & composeKeys(args...);
            }
        template <typename T, typename... Args>
            long composeKeys(T keyFlag, Args... args)
            {
                return keyFlag | composeKeys(args...);
            }
        template <typename T>
            long composeKeys(T t) {
                return t;
            }
    private:
        //expecting max 32 keys, plus possible modifiers
        long keys {0};
        double mouseX {0}, mouseY {0}, mouseScroll {0};
        bool close {false};
};
