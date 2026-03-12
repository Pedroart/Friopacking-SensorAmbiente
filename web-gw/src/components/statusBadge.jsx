/**
 * StatusBadge — Tiny pulsing dot indicator with descriptive icon (online/offline)
 * @param {{ online: boolean, label: string, icon: preact.VNode }} props
 */
export default function StatusBadge({ online, label, icon }) {
    return (
        <div className="status-badge-container" title={label} aria-label={label}>
            <span className="status-icon">{icon}</span>
            <span className={`status-dot ${online ? 'online' : 'offline'}`} />
        </div>
    );
}
