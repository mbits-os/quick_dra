// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

import { LitElement, html, css } from 'lit';
import { customElement } from 'lit/decorators.js';
import { set_subtitle } from '../../integration.js';

import '../title-bar/title-bar.js';

@customElement('qdra-app')
export class QuickDraApp extends LitElement {
  static styles = css`
    :host {
      background-color: rgb(from var(--system-background-color) r g b / 0.95);
      color: var(--system-on-background-color);
      width: 100%;
      height: 100%;
      overflow: hidden;
      display: flex;
      flex-direction: column;
      border-radius: 7px;
      backdrop-filter: blur(24px);
      -webkit-backdrop-filter: blur(24px);

      --primary-color: rgb(73 103 45);
      --system-background-color: rgb(245 246 228);
      --system-on-background-color: rgb(26 29 22);
      --logo-fill: #00923f;

      --sys-button-text-color: #000000;
      --sys-button-text-color-hover: #ebebeb;
      --sys-button-background-color: transparent;
      --sys-button-background-color-hover: rgb(0 0 0 / 0.5);

      --sys-button-text-color-close: var(--sys-button-text-color);
      --sys-button-text-color-hover-close: var(--sys-button-text-color-hover);
      --sys-button-background-color-close: var(--sys-button-background-color);
      --sys-button-background-color-hover-close: rgb(
        from var(--sys-button-background-color-hover) 192 0 0 / alpha
      );

      --web-button-background-color-minimize: #ffbd2e;
      --web-button-background-color-hover-minimize: oklab(
        from var(--web-button-background-color-minimize) calc(L * 0.9) a b
      );

      --web-button-background-color-close: #ff5f57;
      --web-button-background-color-hover-close: oklab(
        from var(--web-button-background-color-close) calc(L * 0.9) a b
      );

      --sys-button-transition: 0.3s;

      @media (prefers-color-scheme: dark) {
        --primary-color: rgb(175 209 140);
        --system-background-color: rgb(26 29 22);
        --system-on-background-color: rgb(226 227 217);
        --logo-fill: oklab(from #00923f calc((L + 1) / 2) a b);

        --sys-button-text-color: var(--sys-button-text-color-hover);
        --sys-button-background-color-hover-close: rgb(
          from var(--sys-button-background-color-hover) 128 0 0 / alpha
        );

        --web-button-background-color-minimize: #ffbd2e;
        --web-button-background-color-hover-minimize: oklab(
          from var(--web-button-background-color-minimize) calc(L * 1.15) a b
        );

        --web-button-background-color-close: #ff5f57;
        --web-button-background-color-hover-close: oklab(
          from var(--web-button-background-color-close) calc(L * 1.15) a b
        );
      }
    }
  `;

  render() {
    return html`
      <title-bar></title-bar>
      <main>
        <button @click=${() => set_subtitle('Settings')}>set title</button>
      </main>
    `;
  }
}
