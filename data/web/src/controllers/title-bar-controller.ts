// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

import { ReactiveController, ReactiveControllerHost } from 'lit';
import { get_title } from '../api/index.js';

export class TitleBarController implements ReactiveController {
  host: ReactiveControllerHost;

  constructor(host: ReactiveControllerHost) {
    this.host = host;
    host.addController(this);
  }

  get title() {
    return get_title();
  }

  hostConnected() {
    window.addEventListener('subtitlechanged', this.#subtitleChanged);
  }

  hostDisconnected() {
    window.removeEventListener('subtitlechanged', this.#subtitleChanged);
  }

  #subtitleChanged = () => {
    this.host.requestUpdate();
  };
}
