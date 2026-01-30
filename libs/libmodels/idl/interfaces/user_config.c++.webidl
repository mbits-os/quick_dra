partial interface config {
    attribute tax_parameters params;

    [throws, nullable] static config parse_yaml([in] path path);
    void debug_print(verbose level);
};

partial interface person {
    attribute string first_name;
    attribute string kind;
    attribute string document;
};

interface tax_parameters {
    [key=currency] attribute record<DOMString, percent> scale;
    attribute currency minimal_pay;
    attribute costs_of_obtaining costs_of_obtaining;
    attribute rates contributions;
};
