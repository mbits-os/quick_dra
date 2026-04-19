// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

import { SubtitleChangedEvent } from './events.js';

let subtitle = '';

export function set_subtitle(next: string) {
  if (next === subtitle) return;
  subtitle = next;
  window.dispatchEvent(new SubtitleChangedEvent(next));
}

export function get_subtitle() {
  return subtitle;
}
