partial interface config {
    [throws, nullable] static config parse_yaml([in] path path);
    [throws, key=year_month, nullable] static record<DOMString, currency> parse_minimal_only([in] path path);
    [throws, key=year_month, nullable] static record<DOMString, currency> parse_minimal_only_from_text([in] string text, [in] string path);

    void debug_print(verbose level);
    [mutable] bool validate();
};

partial interface person {
    attribute string first_name;
    attribute string kind;
    attribute string document;

    [mutable] bool validate();
};

partial interface rate {
    contribution contribution_on(currency amount);
    contribution contribution_on(calc_currency amount);
};
