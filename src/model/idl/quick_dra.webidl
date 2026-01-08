interface config {
    [yaml_name("platnik")] attribute insurer_t insurer;
    [yaml_name("ubezpieczeni")] attribute sequence<insured_t> insured;
    [opt] attribute record<year_month, currency> minimal;
    [yaml_name("parametry"), opt] attribute tax_parameters params;

    [static, throws] config? parse_yaml([in] path path);
    [static, throws] record<year_month, currency>? parse_minimal_only([in] path path);

    void debug_print(verbose level);
    [mutable] bool validate();
};

interface insurer_t {
    [yaml_name("nazwisko")] attribute string last;
    [no_yaml] attribute string first;
    [yaml_name("nip")] attribute string tax_id;
    [yaml_name("pesel")] attribute string social_id;
    [yaml_name("dowod")] attribute string? id_card;
    [yaml_name("paszport")] attribute string? passport;

    [no_yaml] attribute string kind;
    [no_yaml] attribute string document;

    [mutable] bool validate();
};

interface insured_t {
    [yaml_name("nazwisko")] attribute string last;
    [no_yaml] attribute string first;
    [yaml_name("tytul-ubezpieczenia")] attribute insurance_title title;
    [yaml_name("pesel")] attribute string? social_id;
    [yaml_name("dowod")] attribute string? id_card;
    [yaml_name("paszport")] attribute string? passport;
    [yaml_name("wymiar")] attribute ratio? part_time_scale;
    [yaml_name("pensja")] attribute currency? remuneration;

    [no_yaml] attribute string kind;
    [no_yaml] attribute string document;

    [mutable] bool validate();
};

interface rate {
    [yaml_name("calosc")] attribute percent total;
    [yaml_name("ubezpieczony")] attribute percent? insured;

    contribution contribution_on(currency amount);
    contribution contribution_on(calc_currency amount);
};

interface tax_parameters {
    [opt] attribute currency minimal_pay;
    attribute currency cost_of_obtaining;
    attribute currency tax_free_allowance;
    attribute currency free_amount;
    attribute percent tax_rate;
    attribute percent health;
    attribute rate pension_insurance;
    attribute rate disability_insurance;
    attribute rate health_insurance;
    attribute rate accident_insurance;
    attribute rate guaranteed_employee_benefits_fund;
};

interface templates {
    attribute record<string, sequence<report_section>> reports;

    [static, throws] templates? parse_yaml([in] path path);

    [mutable] bool validate();
};

interface report_section {
    attribute string id;
    attribute string? block;
    attribute bool? repeatable;
    [opt] attribute record<unsigned, union<string, sequence<string>>> fields;

    [mutable] bool validate();
};
