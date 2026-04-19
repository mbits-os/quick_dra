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

  render() {
    const platform_name = navigator.platform == 'Win32' ? 'win' : 'web';
    return html`
      <span id="icons"><slot></slot></span>
      <span>${this.controller.title}</span>
      <title-buttons platform=${platform_name}></title-buttons>
    `;
  }
}
