[quote="using yaml::syntax_type;"] 
partial interface config {
    [since_ver=2, throws, mutable] load_status load([in] path path);
    [since_ver=2, throws] static config load_partial([in] path path, [default=true] bool writeable);
    [since_ver=2, throws, mutable] bool store([in] path path, [default="yaml::syntax_type::yaml"] syntax_type syntax);
};

partial interface person {
    attribute string first_name;
    attribute string kind;
    attribute string document;
};

partial interface payer_t {
    [mutable] void postprocess_document_kind();
    [mutable] void preprocess_document_kind();
};

partial interface insured_t {
    [since_ver=2] dated_employment_history lookup([in] year_month date);
    [mutable] void postprocess_document_kind();
    [mutable] void preprocess_document_kind();
};
