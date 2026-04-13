// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

import { ReactiveController, ReactiveControllerHost } from 'lit';
import {
  get_subtitle,
  get_version,
  SubtitleChangedEvent,
} from '../integration.js';

export class TitleBarController implements ReactiveController {
  host: ReactiveControllerHost;
  version: string = '';
  subtitle = get_subtitle();

  constructor(host: ReactiveControllerHost) {
    this.host = host;
    host.addController(this);
    this.#loadVersion();
  }

  get title() {
    if (this.subtitle) {
      return `${this.subtitle} - Quick-DRA ${this.version}`;
    }
    return `Quick-DRA ${this.version}`;
  }

  hostConnected() {
    window.addEventListener('subtitlechanged', this.#subtitleChanged);
  }

  hostDisconnected() {
    window.removeEventListener('subtitlechanged', this.#subtitleChanged);
  }

  updateTitle() {
    this.host.requestUpdate();
    document.getElementsByTagName('title')[0].innerText = this.title;
  }

  setVersion(value: string) {
    this.version = value;
    this.updateTitle();
  }

  setSubtitle(value: string) {
    this.subtitle = value;
    this.updateTitle();
  }

  async #loadVersion() {
    this.setVersion(await get_version());
  }

  #subtitleChanged = (event: SubtitleChangedEvent) => {
    this.setSubtitle(event.subtitle);
  };
}
