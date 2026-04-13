// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

import './components/qdra-app/qdra-app.js';

// const logo = new URL('../../assets/open-wc-logo.svg', import.meta.url).href;

window.addEventListener('load', () => {
  const app = document.createElement('qdra-app');
  document.getElementsByTagName('body')[0].appendChild(app);
});
