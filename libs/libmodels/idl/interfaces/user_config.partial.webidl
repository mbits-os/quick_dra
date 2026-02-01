partial interface config {
    [throws, mutable] load_status load([in] path path);
    [throws, mutable] bool store([in] path path);
};

partial interface person {
    attribute string first_name;
    attribute string kind;
    attribute string document;
};

partial interface payer_t {
    [mutable] void postprocess_document();
    [mutable] void preprocess_document();
};