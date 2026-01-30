[postprocess] interface config {
    [yaml_name=wersja] attribute unsigned short version;
    [yaml_name="płatnik"] attribute insurer_t insurer;
    [yaml_name=ubezpieczeni] attribute sequence<insured_t> insured;
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
