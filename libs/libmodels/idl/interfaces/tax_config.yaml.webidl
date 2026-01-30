[postprocess, merge] interface tax_config {
    attribute unsigned short version;
    [opt] attribute [key=year_month] record<DOMString, [key=currency] record<DOMString, percent>> scale;
    [opt] attribute [key=year_month] record<DOMString, currency> minimal_pay;
    [opt] attribute [key=year_month] record<DOMString, costs_of_obtaining> costs_of_obtaining;
    [opt] attribute [key=year_month] record<DOMString, rates> contributions;
};
