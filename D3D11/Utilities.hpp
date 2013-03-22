#define ReleaseCOM(x) { if(x) { x->Release(); x = 0; } }
