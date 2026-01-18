interface templates {
    attribute record<DOMString, sequence<report_section>> reports;
};

interface report_section {
    attribute string id;
    attribute string? block;
    attribute bool? repeatable;
    [opt] attribute [key=unsigned] record<DOMString, (string or sequence<string>)> fields;
};


enum EnumName {
    "ala", "ma", "kota"
};
