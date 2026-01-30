partial interface tax_config {
    [throws, nullable] static tax_config parse_yaml([in] path path);
    [throws, nullable] static tax_config parse_from_text([in] string text, [in] string path);
    void debug_print(verbose level);
};
