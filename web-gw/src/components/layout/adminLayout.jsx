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
        projects: <svg className="nav-icon" viewBox="0 0 24 24"><path d="M3 13h2v-2H3v2zm0 4h2v-2H3v2zm0-8h2V7H3v2zm4 4h14v-2H7v2zm0 4h14v-2H7v2zM7 7v2h14V7H7z"/></svg>,
        clients: <svg className="nav-icon" viewBox="0 0 24 24"><path d="M16 11c1.66 0 2.99-1.34 2.99-3S17.66 5 16 5c-1.66 0-3 1.34-3 3s1.34 3 3 3zm-8 0c1.66 0 2.99-1.34 2.99-3S9.66 5 8 5C6.34 5 5 6.34 5 8s1.34 3 3 3zm0 2c-2.33 0-7 1.17-7 3.5V19h14v-2.5c0-2.33-4.67-3.5-7-3.5zm8 0c-.29 0-.62.02-.97.05 1.16.84 1.97 1.97 1.97 3.45V19h6v-2.5c0-2.33-4.67-3.5-7-3.5z"/></svg>,
        history: <svg className="nav-icon" viewBox="0 0 24 24"><path d="M14 17H4v2h10v-2zm6-8H4v2h16V9zM4 15h16v-2H4v2zM4 5v2h16V5H4z"/></svg>,
        exceptions: <svg className="nav-icon" viewBox="0 0 24 24"><path d="M12 2C6.48 2 2 6.48 2 12s4.48 10 10 10 10-4.48 10-10S17.52 2 12 2zm1 15h-2v-2h2v2zm0-4h-2V7h2v6z"/></svg>,
        reports: <svg className="nav-icon" viewBox="0 0 24 24"><path d="M19 3H5c-1.1 0-2 .9-2 2v14c0 1.1.9 2 2 2h14c1.1 0 2-.9 2-2V5c0-1.1-.9-2-2-2zm-5 14H7v-2h7v2zm3-4H7v-2h10v2zm0-4H7V7h10v2z"/></svg>,
        users: <svg className="nav-icon" viewBox="0 0 24 24"><path d="M12 12c2.21 0 4-1.79 4-4s-1.79-4-4-4-4 1.79-4 4 1.79 4 4 4zm0 2c-2.67 0-8 1.34-8 4v2h16v-2c0-2.66-5.33-4-8-4z"/></svg>,
        settings: <svg className="nav-icon" viewBox="0 0 24 24"><path d="M19.14 12.94c.04-.3.06-.61.06-.94 0-.32-.02-.64-.07-.94l2.03-1.58a.49.49 0 00.12-.61l-1.92-3.32a.49.49 0 00-.59-.22l-2.39.96c-.5-.38-1.03-.7-1.62-.94l-.36-2.54a.48.48 0 00-.48-.41h-3.84c-.24 0-.43.17-.47.41l-.36 2.54c-.59.24-1.13.57-1.62.94l-2.39-.96a.49.49 0 00-.59.22L2.74 8.87c-.12.21-.08.47.12.61l2.03 1.58c-.05.3-.07.62-.07.94s.02.64.07.94l-2.03 1.58a.49.49 0 00-.12.61l1.92 3.32c.12.22.37.29.59.22l2.39-.96c.5.38 1.03.7 1.62.94l.36 2.54c.05.24.24.41.48.41h3.84c.24 0 .44-.17.47-.41l.36-2.54c.59-.24 1.13-.56 1.62-.94l2.39.96c.22.08.47 0 .59-.22l1.92-3.32c.12-.22.07-.47-.12-.61l-2.01-1.58zM12 15.6A3.6 3.6 0 1115.6 12 3.61 3.61 0 0112 15.6z"/></svg>,
        menu: <svg width="24" height="24" viewBox="0 0 24 24" fill="currentColor"><path d="M3 18h18v-2H3v2zm0-5h18v-2H3v2zm0-7v2h18V6H3z"/></svg>
    };

    return (
        <div className="admin-layout">
            
            {/* Mobile Overlay Background */}
            <div 
                className={`sidebar-overlay ${isMobileMenuOpen ? 'open' : ''}`} 
                onClick={() => setIsMobileMenuOpen(false)}
            ></div>

            <aside className={`admin-sidebar ${isMobileMenuOpen ? 'open' : ''}`}>
                <div className="sidebar-header">
                    <div className="avatar-initials">PA</div>
                    <div className="user-info">
                        <span className="user-name">Pedro Arteta Flores</span>
                        <span className="user-email">parteta@friopacking.pe</span>
                    </div>
                </div>
                
                <nav className="sidebar-nav">
                    {/* The first item "Overview" was active and had an icon, replaced by Dashboard */}
                    <a href="/" className="nav-item active" onClick={() => setIsMobileMenuOpen(false)}>
                        {icons.dashboard} Dashboard
                    </a>
                    <a href="/" className="nav-item" onClick={() => setIsMobileMenuOpen(false)}>
                        {icons.users} Usuarios
                    </a>
                    <a href="/configuracion" className="nav-item" onClick={() => setIsMobileMenuOpen(false)}>
                        {icons.settings} Configuración
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
                        <StatusBadge online={isOnline} label={isOnline ? 'Internet conectado' : 'Sin Internet'} />
                        <StatusBadge online={isDeviceConnected} label={isDeviceConnected ? 'Dispositivos conectados' : 'Sin dispositivos'} />
                    </div>
                    <div className="avatar-initials" style={{ width: '32px', height: '32px', fontSize: '0.8rem' }}>PA</div>
                </div>
            </header>

            <main className="admin-content">
                {children}
            </main>
        </div>
    );
}