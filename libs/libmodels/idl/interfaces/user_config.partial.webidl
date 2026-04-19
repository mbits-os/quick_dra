[quote="enum class emit { yaml, json };"] 
partial interface config {
    [throws, mutable] load_status load([in] path path);
    [throws] static config load_partial([in] path path, [default=true] bool writeable);
    [throws, mutable] bool store([in] path path, [default="emit::yaml"] emit syntax);
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
    [mutable] void postprocess_document_kind();
    [mutable] void preprocess_document_kind();
};
