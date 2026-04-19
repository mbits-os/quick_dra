// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

import './integration.js';
import { WebuiConnectionEvent } from './events.js';
import { WebUI } from './webui.js';

interface Resolvers<T> {
  promise: Promise<T>;
  resolve: (_: T | PromiseLike<T>) => void;
  reject: (_?: unknown) => void;
}

function Promise_withResolvers<T>() {
  const result: Partial<Resolvers<T>> = {};

  result.promise = new Promise<T>((resolve, reject) => {
    result.resolve = resolve;
    result.reject = reject;
  });
  return result as Resolvers<T>;
}

const {
  resolve: webui_ready_resolve,
  reject: webui_read_reject,
  promise: webui_ready_promise,
} = Promise_withResolvers<boolean>();

export async function init_webui() {
  await webui_ready_promise;
}

addEventListener('load', async () => {
  const webui = window.webui;
  if (!webui) {
    webui_read_reject(Error('This is not WebUI client'));
    return;
  }

  webui.setEventCallback(e => {
    if (e == WebUI.Connect.CONNECTED) {
      webui_ready_resolve(true);
      window.dispatchEvent(new WebuiConnectionEvent(true));
    }
  });
});
