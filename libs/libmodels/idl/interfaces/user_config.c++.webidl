partial interface config {
    attribute tax_parameters params;

    [since_ver=2, throws, nullable] static config? parse_yaml([in] path path);
    [since_ver=2] void debug_print(verbose level);
};

partial interface person {
    attribute string first_name;
    attribute string kind;
    attribute string document;
};

partial interface insured_t {
    [since_ver=2] dated_employment_history lookup([in] year_month date);
};

interface tax_parameters {
    [key=currency] attribute record<DOMString, percent> scale;
    attribute currency minimal_pay;
    attribute costs_of_obtaining costs_of_obtaining;
    attribute rates contributions;
};
