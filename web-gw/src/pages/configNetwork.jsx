import { useState } from 'preact/hooks';
import './configNetwork.css';

const TABS = [
    { id: 'cliente',  label: 'Cliente' },
    { id: 'ethernet', label: 'Ethernet' },
    { id: 'host',     label: 'Host' },
];

const DEFAULT_IFACE = { dhcp: true, ip: '', mask: '', gateway: '', dns: '' };

function initialState() {
    return {
        cliente:  { ...DEFAULT_IFACE },
        ethernet: { ...DEFAULT_IFACE },
        host:     { ...DEFAULT_IFACE, dhcp: false, ip: '192.168.4.1', mask: '255.255.255.0' },
    };
}

export default function ConfigNetwork() {
    const [activeTab, setActiveTab] = useState('cliente');
    const [ifaces, setIfaces] = useState(initialState);

    const current = ifaces[activeTab];

    const updateField = (field, value) => {
        setIfaces(prev => ({
            ...prev,
            [activeTab]: { ...prev[activeTab], [field]: value }
        }));
    };

    const handleSave = () => {
        // TODO: POST to device API
        console.log('Save', activeTab, current);
    };

    const handleCancel = () => {
        setIfaces(prev => ({
            ...prev,
            [activeTab]: activeTab === 'host'
                ? { ...DEFAULT_IFACE, dhcp: false, ip: '192.168.4.1', mask: '255.255.255.0' }
                : { ...DEFAULT_IFACE }
        }));
    };

    return (
        <div className="config-network">
            <h2>Configuración de Red</h2>

            {/* Tab pills */}
            <div className="net-tabs" role="tablist">
                {TABS.map(t => (
                    <button
                        key={t.id}
                        role="tab"
                        aria-selected={activeTab === t.id}
                        className={`net-tab ${activeTab === t.id ? 'active' : ''}`}
                        onClick={() => setActiveTab(t.id)}
                    >
                        {t.label}
                    </button>
                ))}
            </div>

            {/* Form card */}
            <div className="net-card" key={activeTab}>
                {/* DHCP toggle */}
                <div className="net-mode-row">
                    <span className="net-mode-label">
                        Modo: <strong>{current.dhcp ? 'DHCP (Automático)' : 'Manual'}</strong>
                    </span>
                    <label className="toggle-switch">
                        <input
                            type="checkbox"
                            checked={current.dhcp}
                            onChange={e => updateField('dhcp', e.target.checked)}
                        />
                        <span className="toggle-slider" />
                    </label>
                </div>

                {/* IP fields */}
                <div className="net-fields">
                    <div className="net-field">
                        <label htmlFor={`ip-${activeTab}`}>Dirección IP</label>
                        <input
                            id={`ip-${activeTab}`}
                            type="text"
                            placeholder="192.168.1.100"
                            value={current.ip}
                            disabled={current.dhcp}
                            onInput={e => updateField('ip', e.target.value)}
                        />
                    </div>
                    <div className="net-field">
                        <label htmlFor={`mask-${activeTab}`}>Máscara de subred</label>
                        <input
                            id={`mask-${activeTab}`}
                            type="text"
                            placeholder="255.255.255.0"
                            value={current.mask}
                            disabled={current.dhcp}
                            onInput={e => updateField('mask', e.target.value)}
                        />
                    </div>
                    <div className="net-field">
                        <label htmlFor={`gw-${activeTab}`}>Puerta de enlace</label>
                        <input
                            id={`gw-${activeTab}`}
                            type="text"
                            placeholder="192.168.1.1"
                            value={current.gateway}
                            disabled={current.dhcp}
                            onInput={e => updateField('gateway', e.target.value)}
                        />
                    </div>
                    <div className="net-field">
                        <label htmlFor={`dns-${activeTab}`}>DNS preferido</label>
                        <input
                            id={`dns-${activeTab}`}
                            type="text"
                            placeholder="8.8.8.8"
                            value={current.dns}
                            disabled={current.dhcp}
                            onInput={e => updateField('dns', e.target.value)}
                        />
                    </div>
                </div>

                {/* Actions */}
                <div className="net-actions">
                    <button className="btn-cancel" onClick={handleCancel} type="button">Cancelar</button>
                    <button className="btn-save" onClick={handleSave} type="button">Guardar</button>
                </div>
            </div>
        </div>
    );
}
