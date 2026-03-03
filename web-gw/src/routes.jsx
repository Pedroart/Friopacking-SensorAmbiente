import Router, { Route } from 'preact-router'
import { useEffect } from 'preact/hooks'

import AdminLayout from './components/layout/adminLayout.jsx'
import CanvasLayout from './components/layout/canvasLayout.jsx'
import Login from './pages/login.jsx' 

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
        </Router>
    )
}