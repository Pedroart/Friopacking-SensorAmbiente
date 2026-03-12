import { useState } from 'preact/hooks';
import { useSnackbar } from '../contexts/SnackbarContext';
import './profile.css';

export default function Profile() {
    const { showSnackbar } = useSnackbar();
    
    const [name, setName] = useState('Pedro Arteta Flores');
    const [email, setEmail] = useState('parteta@friopacking.pe');
    const [oldPassword, setOldPassword] = useState('');
    const [newPassword, setNewPassword] = useState('');
    const [confirmPassword, setConfirmPassword] = useState('');

    const handleSaveProfile = () => {
        // TODO: Update Profile API
        showSnackbar('Perfil actualizado con éxito', 'success');
    };

    const handleUpdatePassword = () => {
        if (newPassword !== confirmPassword) {
            showSnackbar('Las contraseñas nuevas no coinciden', 'error');
            return;
        }
        if (!oldPassword || !newPassword) {
            showSnackbar('Debes llenar todos los campos', 'error');
            return;
        }
        // TODO: Update Password API
        showSnackbar('Contraseña actualizada con éxito', 'success');
        setOldPassword('');
        setNewPassword('');
        setConfirmPassword('');
    };

    return (
        <div className="profile-view">
            <h2>Mi Cuenta</h2>

            <div className="profile-layout">
                {/* Account Details Form */}
                <div className="profile-card">
                    <h3 className="card-title">Detalles Personales</h3>
                    
                    <div className="profile-fields">
                        <div className="profile-field">
                            <label>Nombre Completo</label>
                            <input 
                                type="text" 
                                value={name} 
                                onInput={e => setName(e.target.value)} 
                                placeholder="Tu nombre" 
                            />
                        </div>

                        <div className="profile-field">
                            <label>Correo Electrónico</label>
                            <input 
                                type="email" 
                                value={email} 
                                onInput={e => setEmail(e.target.value)} 
                                placeholder="tu@correo.pe" 
                            />
                        </div>
                    </div>

                    <div className="profile-actions">
                        <button className="btn-save" onClick={handleSaveProfile} type="button">Guardar Detalles</button>
                    </div>
                </div>

                {/* Password Change Form */}
                <div className="profile-card">
                    <h3 className="card-title">Seguridad</h3>
                    
                    <div className="profile-fields">
                        <div className="profile-field">
                            <label>Contraseña Actual</label>
                            <input 
                                type="password" 
                                value={oldPassword} 
                                onInput={e => setOldPassword(e.target.value)} 
                                placeholder="••••••••" 
                            />
                        </div>

                        <div className="profile-field">
                            <label>Nueva Contraseña</label>
                            <input 
                                type="password" 
                                value={newPassword} 
                                onInput={e => setNewPassword(e.target.value)} 
                                placeholder="••••••••" 
                            />
                        </div>

                        <div className="profile-field">
                            <label>Confirmar Nueva Contraseña</label>
                            <input 
                                type="password" 
                                value={confirmPassword} 
                                onInput={e => setConfirmPassword(e.target.value)} 
                                placeholder="••••••••" 
                            />
                        </div>
                    </div>

                    <div className="profile-actions">
                        <button className="btn-save" onClick={handleUpdatePassword} type="button">Actualizar Contraseña</button>
                    </div>
                </div>
            </div>
        </div>
    );
}
