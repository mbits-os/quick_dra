[postprocess] interface config {
    [yaml_name=wersja] attribute unsigned short version;
    [yaml_name="płatnik"] attribute insurer_t insurer;
    [yaml_name=ubezpieczeni] attribute sequence<insured_t> insured;
    [yaml_name=parametry, opt] attribute tax_parameters params;
};

[postprocess] interface insurer_t: person {
    [yaml_name=nip] attribute string tax_id;
    [yaml_name=pesel] attribute string social_id;
};

[postprocess] interface insured_t: person {
    [yaml_name="tytuł ubezpieczenia"] attribute insurance_title title;
    [yaml_name=pesel] attribute string? social_id;
    [yaml_name=wymiar] attribute ratio? part_time_scale;
    [yaml_name=pensja] attribute currency? remuneration;
};

interface person {
    [yaml_name=nazwisko] attribute string last_name;
    [yaml_name="dowód"] attribute string? id_card;
    [yaml_name=paszport] attribute string? passport;
};

interface rate {
    [yaml_name="całość"] attribute percent total;
    [yaml_name=ubezpieczony] attribute percent? insured;
};

interface tax_parameters {
    [yaml_name="płaca minimalna", opt] attribute currency minimal_pay;
    [yaml_name="koszt uzyskania"] attribute currency cost_of_obtaining;
    [yaml_name="kwota wolna od podatku"] attribute currency tax_free_allowance;
    [yaml_name="kwota wolna"] attribute currency free_amount;
    [yaml_name="stawka podatku"] attribute percent tax_rate;
    [yaml_name="stawka zdrowotna"] attribute percent health;
    [yaml_name="ubezpieczenie emerytalne"] attribute rate pension_insurance;
    [yaml_name="ubezpieczenie rentowe"] attribute rate disability_insurance;
    [yaml_name="ubezpieczenie zdrowotne"] attribute rate health_insurance;
    [yaml_name="ubezpieczenie wypadkowe"] attribute rate accident_insurance;
    [yaml_name="FGŚS"] attribute rate guaranteed_employee_benefits_fund;
};
