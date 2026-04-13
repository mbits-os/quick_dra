// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

import { css } from 'lit';

export const styles = css`
  :host {
    height: 36px;
    background: rgba(0, 0, 0, 0.25);
    -webkit-app-region: drag; /* Win32/macOS (Native) */
    --webui-app-region: drag; /* Linux (Custom) */
    display: flex;
    align-items: center;
    justify-content: space-between;
    padding: 0;
    padding-left: 6px;
    flex-shrink: 0;
  }
  .banner {
    height: 100%;
    display: inline-flex;
    align-items: center;
    justify-content: space-between;
    gap: 0.5em;
  }
`;
