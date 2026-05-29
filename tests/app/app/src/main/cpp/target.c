__attribute__((visibility("default")))
int target_get_number(void)
{
    return 42;
}

__attribute__((visibility("default")))
int target_add(int a, int b)
{
    return a + b;
}
