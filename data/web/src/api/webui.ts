// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)
//
// written to reflect external/webui/bridge/webui.ts

/* eslint-disable no-unused-vars */

import BoundApi from './bound_api.js';

export type DataTypes = string | number | boolean | Uint8Array;

export const WebUI = {
  Connect: {
    CONNECTED: 0,
    DISCONNECTED: 1,
  },
} as const;

export interface WebuiBridge extends BoundApi {
  // Methods
  callCore(fn: string, ...args: DataTypes[]): Promise<DataTypes>;

  // -- Public APIs --------------------------
  /**
   * Call a backend function
   *
   * @param fn - binding name
   * @param data - data to be send to the backend function
   * @return - Response of the backend callback string
   * @example - const res = await webui.call("myID", 123, true, "Hi", new Uint8Array([0x42, 0x43, 0x44]))
   */
  call(fn: string, ...args: DataTypes[]): Promise<DataTypes>;
  /**
   * Active or deactivate webui debug logging
   *
   * @param status - log status to set
   */
  setLogging(status: boolean): void;
  /**
   * Encode text into base64 string
   *
   * @param data - text string
   */
  encode(data: string): string;
  /**
   * Decode base64 string into text
   *
   * @param data - base64 string
   */
  decode(data: string): string;
  /**
   * Set a callback to receive events like connect/disconnect
   *
   * @param callback - callback function `myCallback(e)`
   * @example - webui.setEventCallback((e) => {if(e == webui.event.CONNECTED){ ... }});
   */
  setEventCallback(callback: (e: number) => void): void;
  /**
   * Check if UI is connected to the back-end. The connection
   * is done by including `webui.js` virtual file in the HTML.
   *
   * @return - Boolean `true` if connected
   */
  isConnected(): boolean;
  /**
   * Get OS high contrast preference.
   *
   * @return - Boolean `True` if OS is using high contrast theme
   */
  isHighContrast(): Promise<boolean>;
  /**
   * When binding all events on the backend, WebUI blocks all navigation events
   * and sends them to the backend. This API allows you to control that behavior.
   *
   * @param status - Boolean `True` means WebUI will allow navigations
   * @example - webui.allowNavigation(true); // Allow navigation
   * window.location.replace('www.test.com'); // This will now proceed as usual
   */
  allowNavigation(status: boolean): void;
}

// Client extensions
declare global {
  interface Window extends BoundApi {
    readonly webui: WebuiBridge;
  }
}
