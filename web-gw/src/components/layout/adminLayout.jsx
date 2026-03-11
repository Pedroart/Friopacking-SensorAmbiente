import { useState, useEffect } from 'preact/hooks';
import StatusBadge from '../statusBadge.jsx';
import './adminLayout.css';

export default function AdminLayout({ children, title = "Overview" }) {
    const [isMobileMenuOpen, setIsMobileMenuOpen] = useState(false);

    // Internet connection status
    const [isOnline, setIsOnline] = useState(typeof navigator !== 'undefined' ? navigator.onLine : true);
    // Device connection status (placeholder for WebSocket/API)
    const [isDeviceConnected] = useState(false);

    useEffect(() => {
        const goOnline = () => setIsOnline(true);
        const goOffline = () => setIsOnline(false);
        window.addEventListener('online', goOnline);
        window.addEventListener('offline', goOffline);
        return () => {
            window.removeEventListener('online', goOnline);
            window.removeEventListener('offline', goOffline);
        };
    }, []);

    const toggleMobileMenu = () => setIsMobileMenuOpen(!isMobileMenuOpen);
    
    // SVG Icons minimal versions as placeholders
    const icons = {
        overview: <svg className="nav-icon" viewBox="0 0 24 24"><path d="M3 13h8V3H3v10zm0 8h8v-6H3v6zm10 0h8V11h-8v10zm0-18v6h8V3h-8z"/></svg>,
        dashboard: <svg className="nav-icon" viewBox="0 0 24 24"><path d="M4 13h6V5H4v8zm8 6h8V11h-8v8zM4 19h6v-4H4v4zm8-14v4h8V5h-8z"/></svg>,
        sensor: <svg className="nav-icon" viewBox="0 0 24 24"><path d="M12 2a10 10 0 1 0 0 20 10 10 0 0 0 0-20zm0 18a8 8 0 1 1 0-16 8 8 0 0 1 0 16z"/><circle cx="12" cy="12" r="3"/><path d="M12 7v2m0 6v2m-5-5h2m6 0h2"/></svg>,
        projects: <svg className="nav-icon" viewBox="0 0 24 24"><path d="M3 13h2v-2H3v2zm0 4h2v-2H3v2zm0-8h2V7H3v2zm4 4h14v-2H7v2zm0 4h14v-2H7v2zM7 7v2h14V7H7z"/></svg>,
        clients: <svg className="nav-icon" viewBox="0 0 24 24"><path d="M16 11c1.66 0 2.99-1.34 2.99-3S17.66 5 16 5c-1.66 0-3 1.34-3 3s1.34 3 3 3zm-8 0c1.66 0 2.99-1.34 2.99-3S9.66 5 8 5C6.34 5 5 6.34 5 8s1.34 3 3 3zm0 2c-2.33 0-7 1.17-7 3.5V19h14v-2.5c0-2.33-4.67-3.5-7-3.5zm8 0c-.29 0-.62.02-.97.05 1.16.84 1.97 1.97 1.97 3.45V19h6v-2.5c0-2.33-4.67-3.5-7-3.5z"/></svg>,
        history: <svg className="nav-icon" viewBox="0 0 24 24"><path d="M14 17H4v2h10v-2zm6-8H4v2h16V9zM4 15h16v-2H4v2zM4 5v2h16V5H4z"/></svg>,
        exceptions: <svg className="nav-icon" viewBox="0 0 24 24"><path d="M12 2C6.48 2 2 6.48 2 12s4.48 10 10 10 10-4.48 10-10S17.52 2 12 2zm1 15h-2v-2h2v2zm0-4h-2V7h2v6z"/></svg>,
        reports: <svg className="nav-icon" viewBox="0 0 24 24"><path d="M19 3H5c-1.1 0-2 .9-2 2v14c0 1.1.9 2 2 2h14c1.1 0 2-.9 2-2V5c0-1.1-.9-2-2-2zm-5 14H7v-2h7v2zm3-4H7v-2h10v2zm0-4H7V7h10v2z"/></svg>,
        users: <svg className="nav-icon" viewBox="0 0 24 24"><path d="M12 12c2.21 0 4-1.79 4-4s-1.79-4-4-4-4 1.79-4 4 1.79 4 4 4zm0 2c-2.67 0-8 1.34-8 4v2h16v-2c0-2.66-5.33-4-8-4z"/></svg>,
        settings: <svg className="nav-icon" viewBox="0 0 24 24"><path d="M19.14 12.94c.04-.3.06-.61.06-.94 0-.32-.02-.64-.07-.94l2.03-1.58a.49.49 0 00.12-.61l-1.92-3.32a.49.49 0 00-.59-.22l-2.39.96c-.5-.38-1.03-.7-1.62-.94l-.36-2.54a.48.48 0 00-.48-.41h-3.84c-.24 0-.43.17-.47.41l-.36 2.54c-.59.24-1.13.57-1.62.94l-2.39-.96a.49.49 0 00-.59.22L2.74 8.87c-.12.21-.08.47.12.61l2.03 1.58c-.05.3-.07.62-.07.94s.02.64.07.94l-2.03 1.58a.49.49 0 00-.12.61l1.92 3.32c.12.22.37.29.59.22l2.39-.96c.5.38 1.03.7 1.62.94l.36 2.54c.05.24.24.41.48.41h3.84c.24 0 .44-.17.47-.41l.36-2.54c.59-.24 1.13-.56 1.62-.94l2.39.96c.22.08.47 0 .59-.22l1.92-3.32c.12-.22.07-.47-.12-.61l-2.01-1.58zM12 15.6A3.6 3.6 0 1115.6 12 3.61 3.61 0 0112 15.6z"/></svg>,
        menu: <svg width="24" height="24" viewBox="0 0 24 24" fill="currentColor"><path d="M3 18h18v-2H3v2zm0-5h18v-2H3v2zm0-7v2h18V6H3z"/></svg>,
        globe: <svg viewBox="0 0 24 24"><path d="M12 2C6.48 2 2 6.48 2 12s4.48 10 10 10 10-4.48 10-10S17.52 2 12 2zm-1 17.93c-3.95-.49-7-3.85-7-7.93 0-.62.08-1.21.21-1.79L9 15v1c0 1.1.9 2 2 2v1.93zm6.9-2.54c-.26-.81-1-1.39-1.9-1.39h-1v-3c0-.55-.45-1-1-1H8v-2h2c.55 0 1-.45 1-1V7h2c1.1 0 2-.9 2-2v-.41c2.93 1.19 5 4.06 5 7.41 0 2.08-.8 3.97-2.1 5.39z"/></svg>,
        router: <svg viewBox="0 0 24 24"><path d="M19 13H5c-1.1 0-2 .9-2 2v4c0 1.1.9 2 2 2h14c1.1 0 2-.9 2-2v-4c0-1.1-.9-2-2-2zM7 19c-1.1 0-2-.9-2-2s.9-2 2-2 2 .9 2 2-.9 2-2 2zm12 0c-1.1 0-2-.9-2-2s.9-2 2-2 2 .9 2 2-.9 2-2 2zm-2.82-8.58c.2-.21.2-.55 0-.76-1.14-1.21-3.23-2.16-5.18-2.66v-2h2c.55 0 1-.45 1-1s-.45-1-1-1h-2V2c0-.55-.45-1-1-1s-1 .45-1 1v1h-2c-.55 0-1 .45-1 1s.45 1 1 1h2v2c-1.95.5-4.04 1.45-5.18 2.66-.2.21-.2.55 0 .76.2.2.54.21.75 0 2.61-2.71 8.24-2.73 10.86 0 .21.21.55.2.75 0z"/></svg>,
        profile: <svg className="nav-icon" viewBox="0 0 24 24"><path d="M12 12c2.21 0 4-1.79 4-4s-1.79-4-4-4-4 1.79-4 4 1.79 4 4 4zm0 2c-2.67 0-8 1.34-8 4v2h16v-2c0-2.66-5.33-4-8-4z"/></svg> 
    };

    return (
        <div className="admin-layout">
            
            {/* Mobile Overlay Background */}
            <div 
                className={`sidebar-overlay ${isMobileMenuOpen ? 'open' : ''}`} 
                onClick={() => setIsMobileMenuOpen(false)}
            ></div>

            <aside className={`admin-sidebar ${isMobileMenuOpen ? 'open' : ''}`}>
                <a href="/perfil" className="sidebar-header" style={{textDecoration: 'none', color: 'inherit'}}>
                    <div className="avatar-initials">PA</div>
                    <div className="user-info">
                        <span className="user-name">Pedro Arteta Flores</span>
                        <span className="user-email">parteta@friopacking.pe</span>
                    </div>
                </a>
                
                <nav className="sidebar-nav">
                    {/* The first item "Overview" was active and had an icon, replaced by Dashboard */}
                    <a href="/" className={`nav-item ${window.location.pathname === '/' ? 'active' : ''}`} onClick={() => setIsMobileMenuOpen(false)}>
                        {icons.dashboard} Dashboard
                    </a>
                    <a href="/sensores" className={`nav-item ${window.location.pathname === '/sensores' ? 'active' : ''}`} onClick={() => setIsMobileMenuOpen(false)}>
                        {icons.sensor} Asignación de Sensores
                    </a>
                    {/* Removed Usuarios link as part of navigation cleanup */}
                    <a href="/configuracion" className={`nav-item ${window.location.pathname === '/configuracion' ? 'active' : ''}`} onClick={() => setIsMobileMenuOpen(false)}>
                        {icons.settings} Configuración
                    </a>
                    <a href="/perfil" className={`nav-item ${window.location.pathname === '/perfil' ? 'active' : ''}`} onClick={() => setIsMobileMenuOpen(false)}>
                        {icons.profile} Cuenta
                    </a>
                </nav>
            </aside>

            <header className="admin-topbar">
                <div className="topbar-left">
                    <button className="hamburger-btn" onClick={toggleMobileMenu} aria-label="Toggle Menu">
                        {icons.menu}
                    </button>
                    <h2 className="topbar-title">{title}</h2>
                </div>
                <div className="topbar-right">
                    <div className="topbar-badges">
                        <StatusBadge online={isOnline} label={isOnline ? 'Internet conectado' : 'Sin Internet'} icon={icons.globe} />
                        <StatusBadge online={isDeviceConnected} label={isDeviceConnected ? 'Dispositivos conectados' : 'Sin dispositivos'} icon={icons.router} />
                    </div>
                    <a href="/perfil" style={{textDecoration: 'none', color: 'inherit'}}>
                        <div className="avatar-initials" style={{ width: '32px', height: '32px', fontSize: '0.8rem' }}>PA</div>
                    </a>
                </div>
            </header>

            <main className="admin-content">
                {children}
            </main>
        </div>
    );
}