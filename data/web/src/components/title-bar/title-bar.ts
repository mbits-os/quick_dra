// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

import { LitElement, html } from 'lit';
import { customElement } from 'lit/decorators.js';
import { TitleBarController } from '../../controllers/title-bar-controller.js';
import { styles } from './title-bar.styles.js';

import '../fav-icon/fav-icon.js';
import '../title-buttons/title-buttons.js';

@customElement('title-bar')
export class TitleBar extends LitElement {
  static styles = styles;

  controller = new TitleBarController(this);

  #render_banner() {
    return html`
      <fav-icon></fav-icon>
      <span>${this.controller.title}</span>
    `;
  }

  render() {
    const platform_name = navigator.platform == 'Win32' ? 'win' : 'web';
    const banner =
      platform_name === 'win'
        ? html`<span class="banner">${this.#render_banner()}</span>`
        : this.#render_banner();
    return html`
      ${banner}
      <title-buttons platform=${platform_name}></title-buttons>
    `;
  }
}
