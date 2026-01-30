[postprocess] interface tax_config {
    attribute unsigned short version;
    attribute [key=year_month] record<DOMString, [key=currency] record<DOMString, percent>> scale;
    attribute [key=year_month] record<DOMString, currency> minimal_pay;
    attribute [key=year_month] record<DOMString, costs_of_obtaining> costs_of_obtaining;
    attribute [key=year_month] record<DOMString, rates> contributions;
};
