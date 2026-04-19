// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

export interface person {
  last_name?: string;
  first_name?: string;
  id_card?: string;
  passport?: string;
  kind?: string;
  document?: string;
}

export interface payer_t extends person {
  tax_id?: string;
  social_id?: string;
}

export interface insurance_title {
  title_code: string;
  pension_right: number;
  disability_level: number;
}

export interface ratio {
  num: number;
  den: number;
}

export interface insured_t extends person {
  title?: insurance_title;
  social_id?: string;
  part_time_scale?: ratio;
  salary?: number;
}

export interface config {
  payer?: payer_t;
  insured?: insured_t[];
}
