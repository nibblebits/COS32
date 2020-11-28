int abc(int a, char** argv)
{
    char* c = argv[0];
    if (c)
    {
    }
    return 0;
}

int test()
{
    char** testing = 0;
    return abc(50, testing);
}