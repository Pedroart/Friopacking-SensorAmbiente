import { useState } from 'preact/hooks';
import './adminLayout.css';

export default function AdminLayout({ children, title = "Overview" }) {
    const [isMobileMenuOpen, setIsMobileMenuOpen] = useState(false);

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
                    <a href="/" className="nav-item active" onClick={() => setIsMobileMenuOpen(false)}>
                        {icons.overview} Overview
                    </a>
                    <a href="/" className="nav-item" onClick={() => setIsMobileMenuOpen(false)}>
                        {icons.dashboard} Dashboard
                    </a>
                    <a href="/" className="nav-item" onClick={() => setIsMobileMenuOpen(false)}>
                        {icons.projects} Proyectos
                    </a>
                    <a href="/" className="nav-item" onClick={() => setIsMobileMenuOpen(false)}>
                        {icons.clients} Clientes
                    </a>
                    <a href="/" className="nav-item" onClick={() => setIsMobileMenuOpen(false)}>
                        {icons.history} Historial
                    </a>
                    <a href="/" className="nav-item" onClick={() => setIsMobileMenuOpen(false)}>
                        {icons.exceptions} Excepciones
                    </a>
                    <a href="/" className="nav-item" onClick={() => setIsMobileMenuOpen(false)}>
                        {icons.reports} Reportes
                    </a>
                    <a href="/" className="nav-item" onClick={() => setIsMobileMenuOpen(false)}>
                        {icons.users} Usuarios
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
                <div className="avatar-initials" style={{ width: '32px', height: '32px', fontSize: '0.8rem' }}>PA</div>
            </header>

            <main className="admin-content">
                {children}
            </main>
        </div>
    );
}