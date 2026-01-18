interface config {
    [yaml_name=platnik] attribute insurer_t insurer;
    [yaml_name=ubezpieczeni] attribute sequence<insured_t> insured;
    [opt] attribute [key=year_month] record<DOMString, currency> minimal;
    [yaml_name=parametry, opt] attribute tax_parameters params;
};

[postprocess] interface insurer_t: person {
    [yaml_name=nip] attribute string tax_id;
    [yaml_name=pesel] attribute string social_id;
};

[postprocess] interface insured_t: person {
    [yaml_name=tytul-ubezpieczenia] attribute insurance_title title;
    [yaml_name=pesel] attribute string? social_id;
    [yaml_name=wymiar] attribute ratio? part_time_scale;
    [yaml_name=pensja] attribute currency? remuneration;
};

interface person {
    [yaml_name=nazwisko] attribute string last_name;
    [yaml_name=dowod] attribute string? id_card;
    [yaml_name=paszport] attribute string? passport;
};

interface rate {
    [yaml_name=calosc] attribute percent total;
    [yaml_name=ubezpieczony] attribute percent? insured;
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
