import Router, { Route, route } from 'preact-router'
import { useEffect, useState } from 'preact/hooks'

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
  return null
}

function ProtectedRoute({ component: Component, layout: Layout }) {
    const [isAuthenticated, setIsAuthenticated] = useState(false);
    const [isLoading, setIsLoading] = useState(true);

    useEffect(() => {
        // Enforce Login session via sessionStorage tracking
        const user = sessionStorage.getItem('auth_user');
        if (!user) {
            route('/login', true); // replace history step
        } else {
            setIsAuthenticated(true);
        }
        setIsLoading(false);
    }, []);

    if (isLoading) return null; // Avoid rendering flash before redirect

    if (!isAuthenticated) return null;

    return (
        <Layout>
            <ScrollToTop />
            <Component />
        </Layout>
    );
}

export default function Routes() {
    return (
        <Router>
            {/* Login - Public route */}
            <Route path='/login' component={
                () => <WithLayout layout={CanvasLayout} > <ScrollToTop/> <Login/> </WithLayout>
            } />
            
            {/* Dashboard (Admin Layout) */}
            <Route path='/' component={
                () => <ProtectedRoute layout={AdminLayout} component={Dashboard} />
            } />

            {/* Asignación de Sensores */}
            <Route path='/sensores' component={
                () => <ProtectedRoute layout={AdminLayout} component={SensorLayout} />
            } />

            {/* Configuración de Red */}
            <Route path='/configuracion' component={
                () => <ProtectedRoute layout={AdminLayout} component={ConfigNetwork} />
            } />

            {/* Perfil de Usuario */}
            <Route path='/perfil' component={
                () => <ProtectedRoute layout={AdminLayout} component={Profile} />
            } />
        </Router>
    )
}