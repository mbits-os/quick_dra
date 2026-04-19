// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

import { LitElement, html } from 'lit';
import { customElement, query } from 'lit/decorators.js';
import { MdMenu } from '@material/web/menu/menu.js';
import { set_subtitle } from '../../api/index.js';

import { styles } from './qdra-app.styles.js';

import '../title-bar/title-bar.js';
import '@material/web/menu/menu-item.js';
import '@material/web/iconbutton/icon-button.js';
import '@material/web/icon/icon.js';

@customElement('qdra-app')
export class QuickDraApp extends LitElement {
  static styles = styles;

  @query('#app-menu')
  app_menu?: MdMenu;

  render() {
    return html`
      <title-bar>
        <md-icon-button @click=${() => location.reload()}>
          <md-icon>refresh</md-icon>
        </md-icon-button>
      </title-bar>
      <main>
        <button @click=${() => set_subtitle('Settings')}>set title</button>
      </main>
    `;
  }
}
