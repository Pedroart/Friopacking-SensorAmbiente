import Router, { Route } from 'preact-router'
import { useEffect } from 'preact/hooks'

import AdminLayout from './components/layout/adminLayout.jsx'
import CanvasLayout from './components/layout/canvasLayout.jsx'
import Login from './pages/login.jsx'
import ConfigNetwork from './pages/configNetwork.jsx'
import Profile from './pages/profile.jsx'
import SensorLayout from './pages/sensorLayout.jsx'
import Dashboard from './pages/dashboard.jsx'

function WithLayout( {layout: Layout, children} ){
    return <Layout>{children}</Layout>
}

function ScrollToTop() {
  useEffect(() => window.scrollTo(0, 0), [])
  return true
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
                () => <WithLayout layout={AdminLayout} > <ScrollToTop/> <Dashboard/> </WithLayout>
            } />

            {/* Asignación de Sensores */}
            <Route path='/sensores' component={
                () => <WithLayout layout={AdminLayout} > <ScrollToTop/> <SensorLayout/> </WithLayout>
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