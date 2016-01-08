import std.string;
import std.stdio;
import std.c.process;

void runtimeError(string message)
{
    writeln("runtime error: " ~ message);
    exit(-1);
}

void validationError(string message)
{
    writeln("validation error: " ~ message);
    exit(-2);
}

void assertEquals(T)(T actual, T expected, string message)
{
    if (actual != expected)
    {
        validationError(format("%s\nactual value %s, expected value %s",
            message, actual, expected));
    }
}

ulong combineHashes(ulong hash1, ulong hash2)
{
    return hash1 ^ (hash2 + 0x9e3779b9 + (hash1 << 6) + (hash1 >> 2));
}