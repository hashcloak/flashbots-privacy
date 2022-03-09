#ifdef __MACH__
#define cvariable(var) _##var
#else
#define cvariable(var) var
#endif
