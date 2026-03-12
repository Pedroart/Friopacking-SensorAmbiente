import { useState, useEffect } from 'preact/hooks';
import { useSnackbar } from '../contexts/SnackbarContext';
import './profile.css';

export default function Profile() {
    const { showSnackbar } = useSnackbar();
    
    const [name, setName] = useState('...');
    const [role, setRole] = useState('...'); // 3 rules: viewer, operator, admin
    const [oldPassword, setOldPassword] = useState('');
    const [newPassword, setNewPassword] = useState('');
    const [confirmPassword, setConfirmPassword] = useState('');

    useEffect(() => {
        // Map integer to string roles
        const roleMap = {
            '0': 'Viewer',
            '1': 'Operator',
            '2': 'Admin'
        };

        const sessionUser = sessionStorage.getItem('auth_user');
        const sessionRoleStr = sessionStorage.getItem('auth_role');
        
        if (sessionUser) {
            setName(sessionUser.charAt(0).toUpperCase() + sessionUser.slice(1));
        }
        
        if (sessionRoleStr && roleMap[sessionRoleStr]) {
            setRole(roleMap[sessionRoleStr]);
        }
    }, []);

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
                            <label>Usuario</label>
                            <input 
                                type="text" 
                                value={name} 
                                disabled
                                style={{ opacity: 0.7, cursor: 'not-allowed' }}
                            />
                        </div>

                        <div className="profile-field">
                            <label>Rol</label>
                            <input 
                                type="text" 
                                value={role} 
                                disabled
                                style={{ opacity: 0.7, cursor: 'not-allowed', color: 'var(--accent-color, #3b82f6)', fontWeight: 600 }}
                            />
                        </div>
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
