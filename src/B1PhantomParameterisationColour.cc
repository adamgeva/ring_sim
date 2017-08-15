#include "B1PhantomParameterisationColour.hh"

#include "globals.hh"
#include "G4VisAttributes.hh"
#include "G4Material.hh"
#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"

B1PhantomParameterisationColour::B1PhantomParameterisationColour()
: G4PhantomParameterisation()
{
    ReadColourData();
    //TODO: originally it was set to false
    SetSkipEqualMaterials(false);
}

B1PhantomParameterisationColour::~B1PhantomParameterisationColour()
{
}

void B1PhantomParameterisationColour::ReadColourData()
{
    //----- Add a G4VisAttributes for materials not defined in file;
    G4VisAttributes* blankAtt = new G4VisAttributes;
    blankAtt->SetVisibility( FALSE );
    fColours["Default"] = blankAtt;
    
    //----- Read file
    G4String colourFile = "ColourMap.dat";
    std::ifstream fin(colourFile.c_str());
    G4int nMate;
    G4String mateName;
    G4double cred, cgreen, cblue, copacity;
    fin >> nMate;
    for( G4int ii = 0; ii < nMate; ii++ ){
        fin >> mateName >> cred >> cgreen >> cblue >> copacity;
        G4Colour colour( cred, cgreen, cblue, copacity );
        G4VisAttributes* visAtt = new G4VisAttributes( colour );
        //visAtt->SetForceSolid(true);
        fColours[mateName] = visAtt;
    }
    
}

G4Material* B1PhantomParameterisationColour::
ComputeMaterial(const G4int copyNo, G4VPhysicalVolume * physVol, const G4VTouchable *)
{
    G4Material* mate = G4PhantomParameterisation::ComputeMaterial( copyNo, physVol, 0 );
    if( physVol ) {
        G4String mateName = mate->GetName();
        std::string::size_type iuu = mateName.find("__");
        if( iuu != std::string::npos ) {
            mateName = mateName.substr( 0, iuu );
        }
        std::map<G4String,G4VisAttributes*>::const_iterator ite =
          fColours.find(mateName);
        if( ite != fColours.end() ){
            physVol->GetLogicalVolume()->SetVisAttributes( (*ite).second );
        } else {
            physVol->GetLogicalVolume()->SetVisAttributes( 
                                  (*(fColours.begin()) ).second );
            // set it as unseen
        }
    }
    
    return mate;
}
