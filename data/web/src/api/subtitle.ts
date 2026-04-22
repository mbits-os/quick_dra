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

function get_version() {
  const uiVersion = new URLSearchParams(window.location.search).get('version');
  return uiVersion ?? '@@VERSION@@';
}

export function get_title() {
  const version = get_version();
  if (subtitle) {
    return `${subtitle} - Quick-DRA (${version})`;
  }
  return `Quick-DRA (${version})`;
}
