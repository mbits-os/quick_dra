// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

import { config } from './models.js';

// eslint-disable-next-line
export type JsonOf<T> = string;

export default interface BoundApi {
  readonly close_win: () => void;
  readonly minimize: () => void;
  readonly get_config: () => Promise<JsonOf<config>>;
}
