// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

import { css } from 'lit';

export const styles = css`
  :host {
    display: flex;
    height: 100%;
    align-items: stretch;
  }

  .container {
    display: flex;
    align-items: center;
    &.web {
      gap: 0.5em;
      margin-right: 1em;
    }
  }

  button {
    border: none;
    margin: 0;
    padding: 0;
    width: auto;
    overflow: visible;

    background: transparent;

    /* inherit font & color from ancestor */
    color: inherit;
    font: inherit;

    line-height: normal;

    /* Corrects font smoothing for webkit */
    -webkit-font-smoothing: inherit;
    -moz-osx-font-smoothing: inherit;

    -webkit-appearance: none;

    --transition: var(--sys-button-transition, 0.5s);

    --text-color-normal: var(--sys-button-text-color, #000000);
    --background-color-normal: var(--sys-button-background-color, transparent);

    --text-color-hover: var(--sys-button-text-color-hover, #ebebeb);
    --background-color-hover: var(
      --sys-button-background-color-hover,
      rgb(0 0 0 / 0.6)
    );

    --text-color-close: var(--sys-button-text-color-close, var(--text-color));
    --background-color-close: var(
      --sys-button-background-color-close,
      var(--sys-button-background-color)
    );

    --text-color-hover-close: var(
      --sys-button-text-color-hover-close,
      var(--sys-button-text-color-hover)
    );
    --background-color-hover-close: var(
      --sys-button-background-color-hover-close,
      rgb(from var(--background-color-hover) 128 0 0 / A)
    );
  }

  .button {
    display: inline-block;
    line-height: 1;
    font-size: 0.7em;
    width: 4em;
    height: 100%;
    cursor: pointer;
    color: var(--text-color);
    background-color: var(--background-color);
    transition: all var(--transition) ease-out;

    --text-color: var(--text-color-normal);
    --background-color: var(--background-color-normal);

    &:hover {
      --text-color: var(--text-color-hover);
      --background-color: var(--background-color-hover);
    }
  }

  .win .button {
    font-family: 'Segoe MDL2 Assets';

    &.close {
      --text-color-normal: var(--text-color-close);
      --background-color-normal: var(--background-color-close);
      --text-color-hover: var(--text-color-hover-close);
      --background-color-hover: var(--background-color-hover-close);
    }
  }

  .web .button {
    border-radius: 50%;
    padding: auto;
    width: 20px;
    height: 20px;

    span {
      visibility: hidden;
    }

    &:hover {
      transform: scale(1.1);
    }

    &.minimize {
      --background-color-normal: var(
        --web-button-background-color-minimize,
        yellow
      );
      --background-color-hover: var(
        --web-button-background-color-hover-minimize,
        oklab(from var(--background-color) calc(L * 0.9) a b)
      );
    }

    &.close {
      --background-color-normal: var(
        --web-button-background-color-close,
        yellow
      );
      --background-color-hover: var(
        --web-button-background-color-hover-close,
        oklab(from var(--background-color) calc(L * 0.9) a b)
      );
    }
  }
`;
