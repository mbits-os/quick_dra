// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

import { LitElement, html } from 'lit';
import { customElement, property } from 'lit/decorators.js';
import { app } from '../../integration.js';
import { styles } from './title-buttons.styles.js';
import { repeat } from 'lit/directives/repeat.js';

interface Button {
  class_name: string;
  win_icon: string;
  onclick: () => void;
}

const buttons: Button[] = [
  {
    class_name: 'minimize',
    win_icon: '\ue921',
    onclick: () => app.minimize?.(),
  },
  { class_name: 'close', win_icon: '\ue8bb', onclick: () => app.close_win?.() },
];

@customElement('title-buttons')
export class TitleButtons extends LitElement {
  static styles = styles;

  @property({ type: String }) platform?: string = 'web';

  render() {
    return html`
      <span class="container ${this.platform}">
        ${repeat(
          buttons,
          ({ class_name, win_icon, onclick }) =>
            html`<button class="button ${class_name}" @click=${onclick}>
              <span>${win_icon}</span>
            </button>`,
        )}
      </span>
    `;
  }
}
