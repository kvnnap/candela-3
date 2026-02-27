export module core.version;

export namespace candela::version 
{
    extern const char* Commit;
    extern const char* Date;
    extern const bool Dirty;

    const char* GetCommitSummary();
}
