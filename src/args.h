struct Args
{
    const char* url;
    const char* outfile;
};

struct Args parse_args(int argc, char** argv);
