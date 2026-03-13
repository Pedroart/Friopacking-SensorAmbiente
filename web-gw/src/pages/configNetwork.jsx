import { useState, useEffect } from 'preact/hooks';
import './configNetwork.css';
import { networkApi } from '../services/networkApi';

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

const mapFromApi = (netObj) => {
    if (!netObj) return { ...DEFAULT_IFACE };
    return {
        dhcp: Boolean(netObj.dhcp),
        ip: netObj.ip || '',
        mask: netObj.subnet || '',
        gateway: netObj.gateway || '',
        dns: netObj.dns1 || ''
    };
};

const mapToApi = (uiObj) => {
    const payload = { dhcp: uiObj.dhcp };
    if (!uiObj.dhcp) {
        payload.ip = uiObj.ip;
        payload.subnet = uiObj.mask;
        payload.gateway = uiObj.gateway;
        if (uiObj.dns) payload.dns1 = uiObj.dns;
    }
    return payload;
};

export default function ConfigNetwork() {
    const [activeTab, setActiveTab] = useState('cliente');
    const [ifaces, setIfaces] = useState(initialState);
    const [loading, setLoading] = useState(true);
    const [saving, setSaving] = useState(false);
    const [error, setError] = useState(null);
    const [successMsg, setSuccessMsg] = useState(null);

    const loadConfig = async () => {
        try {
            setLoading(true);
            setError(null);
            const data = await networkApi.getNetworkConfig();
            setIfaces({
                cliente: mapFromApi(data.sta?.net),
                ethernet: mapFromApi(data.eth?.net),
                host: mapFromApi(data.ap?.net)
            });
        } catch (err) {
            setError(err.message || 'Error al cargar la configuración de red');
        } finally {
            setLoading(false);
        }
    };

    useEffect(() => {
        loadConfig();
    }, []);

    const current = ifaces[activeTab];

    const updateField = (field, value) => {
        setIfaces(prev => ({
            ...prev,
            [activeTab]: { ...prev[activeTab], [field]: value }
        }));
    };

    const handleSave = async () => {
        try {
            setSaving(true);
            setError(null);
            setSuccessMsg(null);

            const netPayload = mapToApi(current);

            if (activeTab === 'ethernet') {
                await networkApi.updateEthernet({ net: netPayload });
            } else if (activeTab === 'cliente') {
                await networkApi.updateSTA({ net: netPayload });
            } else if (activeTab === 'host') {
                await networkApi.updateAP({ net: netPayload });
            }

            setSuccessMsg('Configuración guardada correctamente');
            setTimeout(() => setSuccessMsg(null), 3000);
        } catch (err) {
            setError(err.message || 'Error al guardar la configuración');
        } finally {
            setSaving(false);
        }
    };

    const handleCancel = () => {
        // Restaurar estado cargando nuevamente los datos
        loadConfig();
    };

    return (
        <div className="config-network">
            <h2>Configuración de Red</h2>

            {/* Mensajes de Feedback */}
            {error && <div className="alert-error" style={{ color: '#d32f2f', background: '#ffebee', padding: '10px', borderRadius: '4px', marginBottom: '15px' }}>{error}</div>}
            {successMsg && <div className="alert-success" style={{ color: '#2e7d32', background: '#e8f5e9', padding: '10px', borderRadius: '4px', marginBottom: '15px' }}>{successMsg}</div>}

            {/* Tab pills */}
            <div className="net-tabs" role="tablist">
                {TABS.map(t => (
                    <button
                        key={t.id}
                        role="tab"
                        aria-selected={activeTab === t.id}
                        className={`net-tab ${activeTab === t.id ? 'active' : ''}`}
                        onClick={() => {
                            setActiveTab(t.id);
                            setError(null);
                            setSuccessMsg(null);
                        }}
                        disabled={loading || saving}
                    >
                        {t.label}
                    </button>
                ))}
            </div>

            {/* Form card */}
            <div className="net-card" key={activeTab}>
                {loading ? (
                    <div className="loading-state" style={{ padding: '20px', textAlign: 'center', color: '#666' }}>
                        Cargando configuración...
                    </div>
                ) : (
                    <>
                        {/* DHCP toggle */}
                        <div className="net-mode-row">
                            <span className="net-mode-label">
                                Modo: <strong>{current.dhcp ? 'DHCP (Automático)' : 'Manual'}</strong>
                            </span>
                            <div 
                                className={`custom-toggle ${current.dhcp ? 'checked' : ''}`}
                                onClick={() => updateField('dhcp', !current.dhcp)}
                                role="switch"
                                aria-checked={current.dhcp}
                                tabIndex={0}
                                onKeyDown={e => {
                                    if (e.key === 'Enter' || e.key === ' ') {
                                        e.preventDefault();
                                        updateField('dhcp', !current.dhcp);
                                    }
                                }}
                            >
                                <div className="toggle-knob" />
                            </div>
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
                                    disabled={current.dhcp || saving}
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
                                    disabled={current.dhcp || saving}
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
                                    disabled={current.dhcp || saving}
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
                                    disabled={current.dhcp || saving}
                                    onInput={e => updateField('dns', e.target.value)}
                                />
                            </div>
                        </div>

                        {/* Actions */}
                        <div className="net-actions">
                            <button className="btn-cancel" onClick={handleCancel} type="button" disabled={saving}>Cancelar</button>
                            <button className="btn-save" onClick={handleSave} type="button" disabled={saving}>
                                {saving ? 'Guardando...' : 'Guardar'}
                            </button>
                        </div>
                    </>
                )}
            </div>
        </div>
    );
}
