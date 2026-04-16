// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

import { LitElement, html } from 'lit';
import { customElement, query } from 'lit/decorators.js';
import { MdMenu } from '@material/web/menu/menu.js';
import { set_subtitle } from '../../integration.js';

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

  #toggleMenu = () => {
    if (!this.app_menu) return;
    this.app_menu.open = !this.app_menu.open;
  };

  render() {
    return html`
      <title-bar>
        <md-icon-button
          aria-label="Opcje"
          id="menu-toggle"
          @click=${this.#toggleMenu}
        >
          <md-icon>menu</md-icon>
        </md-icon-button>
        <!--span style="position: relative">
          <md-menu id="app-menu" anchor="menu-toggle">
            <md-menu-item>This</md-menu-item>
            <md-menu-item>That</md-menu-item>
            <md-menu-item>Something else</md-menu-item>
          </md-menu>
        </span-->
      </title-bar>
      <main>
        <button @click=${() => set_subtitle('Settings')}>set title</button>
      </main>
    `;
  }
}
