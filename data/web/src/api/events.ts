// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

export class SubtitleChangedEvent extends Event {
  subtitle: string;
  constructor(subtitle: string) {
    super('subtitlechanged');
    this.subtitle = subtitle;
  }
}

export class WebuiConnectionEvent extends Event {
  connected: boolean;
  constructor(connected: boolean) {
    super('webuiconnection');
    this.connected = connected;
  }
}

declare global {
  interface WindowEventMap {
    subtitlechanged: SubtitleChangedEvent;
    webuiconnection: WebuiConnectionEvent;
  }
}
