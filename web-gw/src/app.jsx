import './app.css'
import Routes from './routes'
import { SnackbarProvider } from './contexts/SnackbarContext'

export function App() {
  
  return (
    <SnackbarProvider>
      <Routes/>
    </SnackbarProvider>
  )
}
