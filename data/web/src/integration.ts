// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

export class SubtitleChangedEvent extends Event {
  subtitle: string;
  constructor(subtitle: string) {
    super('subtitlechanged');
    this.subtitle = subtitle;
  }
}

export interface WindowIncoming {
  close_win?: () => void;
  minimize?: () => void;
  get_version?: () => Promise<string>;
}

type App = typeof window & WindowIncoming;

export const app = window as App;

function version_getter(
  resolve: (_: string) => void,
  data: { timer: number; then: number },
) {
  const self = () => {
    if (Date.now() - data.then > 10_000) {
      clearTimeout(data.timer);
      console.log('-> <fallback>');
      resolve('');
      return;
    }

    const promise = app.get_version?.();
    if (promise === undefined) {
      return;
    }
    clearTimeout(data.timer);
    promise.then(value => {
      if (value === undefined) {
        console.log('no value....');
        data.timer = setInterval(self, 100);
      } else {
        console.log('->', value);
        resolve(value);
      }
    });
  };
  return self;
}

let subtitle = '';

export function set_subtitle(next: string) {
  if (next === subtitle) return;
  subtitle = next;
  window.dispatchEvent(new SubtitleChangedEvent(next));
}

export function get_subtitle() {
  return subtitle;
}

export async function get_version() {
  console.log('>>> get_version()');
  return new Promise<string>(resolve => {
    const data = { timer: -1, then: Date.now() };
    const cb = version_getter(resolve, data);
    data.timer = setInterval(cb, 100);
  });
}

declare global {
  interface WindowEventMap {
    subtitlechanged: SubtitleChangedEvent;
  }
}
