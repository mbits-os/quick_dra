// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

import { config } from './models.js';
import './webui.js';

export function get_value<T>(call: () => Promise<T>) {
  return new Promise<T | undefined>((resolve, reject) => {
    if (!window.webui.isConnected()) {
      reject(new Error('WebUI has not connected yet'));
    }
    call().then(resolve, reject);
  });
}

export async function get_version() {
  return (await get_value(window.get_version)) ?? '';
}

export async function get_config() {
  try {
    return JSON.parse((await get_value(window.get_config)) ?? '') as config;
  } catch (error) {
    console.error('Error while calling get_config():', error);
    return null;
  }
}
