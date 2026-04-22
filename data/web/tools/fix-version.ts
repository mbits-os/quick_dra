// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

import packageJson from '../package.json' with { type: 'json' };
import { replaceInFile } from 'replace-in-file';

replaceInFile({
  files: ['dist/index.html', 'dist/index.js'],
  from: '@@VERSION@@',
  to: packageJson.version,
});
