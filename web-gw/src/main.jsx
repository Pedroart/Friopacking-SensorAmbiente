import { render } from 'preact'
import './index.css'
import { App } from './app.jsx'
import '@picocss/pico/css/pico.min.css';

document.documentElement.setAttribute('data-theme', 'dark')
render(<App />, document.getElementById('app'))
