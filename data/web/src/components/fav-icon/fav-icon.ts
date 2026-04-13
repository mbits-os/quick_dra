// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

import {
  LitElement,
  css,
  ReactiveController,
  ReactiveControllerHost,
} from 'lit';

import { customElement } from 'lit/decorators.js';
import { unsafeSVG } from 'lit/directives/unsafe-svg.js';

class FaviconController implements ReactiveController {
  host: ReactiveControllerHost;

  svg: string = '';

  constructor(host: ReactiveControllerHost) {
    this.host = host;
    this.host.addController(this);
  }

  async hostConnected(): Promise<void> {
    const response = await fetch('/favicon.svg');
    this.svg = await response.text();
    this.host.requestUpdate();
  }
}

@customElement('fav-icon')
export class Favicon extends LitElement {
  controller = new FaviconController(this);

  static styles = css`
    :host {
      height: 24px;
      width: 24px;
      display: inline-block;

      svg {
        width: 100%;
        height: 100%;
        fill: var(--logo-fill);
      }
    }
  `;

  render() {
    const { svg } = this.controller;
    return unsafeSVG(svg);
  }
}
