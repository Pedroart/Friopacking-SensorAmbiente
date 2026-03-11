import Router, { Route } from 'preact-router'
import { useEffect } from 'preact/hooks'

import AdminLayout from './components/layout/adminLayout.jsx'
import CanvasLayout from './components/layout/canvasLayout.jsx'
import Login from './pages/login.jsx'
import ConfigNetwork from './pages/configNetwork.jsx'
import Profile from './pages/profile.jsx'

function WithLayout( {layout: Layout, children} ){
    return <Layout>{children}</Layout>
}

function ScrollToTop() {
  useEffect(() => window.scrollTo(0, 0), [])
  return true
}

// Temporary placeholder for the dashboard content
function DashboardPlaceholder() {
    return (
        <div style={{ color: '#94a3b8' }}>
            <h2>Overview</h2>
            <p>Seleccione un marcador en el mapa o contenido desde el menú lateral.</p>
        </div>
    );
}


export default function Routes() {
    return (
        <Router>
            {/* Login */}
            <Route path='/login' component={
                () => <WithLayout layout={CanvasLayout} > <ScrollToTop/> <Login/> </WithLayout>
            } />
            
            {/* Dashboard (Admin Layout) */}
            <Route path='/' component={
                () => <WithLayout layout={AdminLayout} > <ScrollToTop/> <DashboardPlaceholder/> </WithLayout>
            } />

            {/* Configuración de Red */}
            <Route path='/configuracion' component={
                () => <WithLayout layout={AdminLayout} > <ScrollToTop/> <ConfigNetwork/> </WithLayout>
            } />

            {/* Perfil de Usuario */}
            <Route path='/perfil' component={
                () => <WithLayout layout={AdminLayout} > <ScrollToTop/> <Profile/> </WithLayout>
            } />
        </Router>
    )
}