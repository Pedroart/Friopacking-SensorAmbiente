/**
 * StatusBadge — Tiny pulsing dot indicator (online/offline)
 * @param {{ online: boolean, label: string }} props
 */
export default function StatusBadge({ online, label }) {
    return (
        <span
            className={`status-badge ${online ? 'online' : 'offline'}`}
            title={label}
            aria-label={label}
        />
    );
}
