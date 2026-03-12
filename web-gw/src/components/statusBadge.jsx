/**
 * StatusBadge — Tiny pulsing dot indicator with descriptive icon (online/offline)
 * @param {{ online: boolean, label: string, description: string, icon: preact.VNode }} props
 */
export default function StatusBadge({ online, label, description, icon }) {
    return (
        <div className="status-badge-container">
            <span className="status-icon">{icon}</span>
            <span className={`status-dot ${online ? 'online' : 'offline'}`} />
            
            <div className="status-tooltip">
                <div className="status-tooltip-title">{label}</div>
                <div className="status-tooltip-desc">{description}</div>
            </div>
        </div>
    );
}
